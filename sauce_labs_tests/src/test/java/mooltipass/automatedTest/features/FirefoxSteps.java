package mooltipass.automatedTest.features;

import cucumber.api.java.en.Then;
import cucumber.api.java.en.When;
import junit.framework.Assert;
import mooltipass.automatedTest.config.WebDriverFactory;
import mooltipass.automatedTest.pageObjects.Firefox;

public class FirefoxSteps {
	Firefox firefox= new Firefox(WebDriverFactory.get());

	@When("I login firefox with '(.*)'")
	public void login(String username){
		firefox.goToLogin();
		firefox.enterEmail(username);
		String password =System.getenv().get("MOZPASS");
		firefox.enterPassword(password);
		firefox.submit();
		
	}
	@When("I go to firefox login page")
	public void pressLogin(){
		firefox.goToLogin();
		Assert.assertTrue("Expected to be at login page", firefox.checkAtLoginPage());
		
	}
	
	@When("I click the sign in button")
	public void pressSignIn(){
		firefox.sleep(4000);
		firefox.submit();
	}
	
	@Then("I should be logged in firefox")
	public void checkLogin(){
		Assert.assertTrue("Expected User to be logged in",firefox.checkLogin());
	}
	
	@When("I logout firefox")
	public void pressLogout(){
		firefox.clickuser();
		firefox.logout();
	}
}
