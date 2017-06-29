package mooltipass.automatedTest.features;

import cucumber.api.java.en.Then;
import cucumber.api.java.en.When;
import junit.framework.Assert;
import mooltipass.automatedTest.config.WebDriverFactory;
import mooltipass.automatedTest.pageObjects.Firefox;

public class FirefoxSteps {
	Firefox firefox= new Firefox(WebDriverFactory.get());

	@When("I login firefox with '(.*)' and '(.*)'")
	public void login(String username,String password){
		firefox.goToLogin();
		firefox.enterEmail(username);
		firefox.enterPassword(password);
		firefox.submit();
		
	}
	@When("I go to firefox login page")
	public void pressLogin(){
		firefox.goToLogin();
		Assert.assertTrue("Expected to be at login page", firefox.checkAtLoginPage());
		
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
