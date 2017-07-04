package mooltipass.automatedTest.features;

import cucumber.api.java.en.Then;
import cucumber.api.java.en.When;
import junit.framework.Assert;
import mooltipass.automatedTest.config.WebDriverFactory;
import mooltipass.automatedTest.pageObjects.Tripadvisor;

public class TripadvisorSteps {

	Tripadvisor tripadvisor = new Tripadvisor(WebDriverFactory.get());
	
	@When("I login tripadvisor with '(.*)'")
	public void login(String username){
		tripadvisor.goToLogin();
		tripadvisor.enterEmail(username);
		String password =System.getenv().get("TRIPPASS");
		tripadvisor.enterPassword(password);
		tripadvisor.submit();
		
	}

	@Then("I should be logged in tripadvisor")
	public void checkLogin(){
		Assert.assertTrue("Expected User to be logged in",tripadvisor.checkLogin());
	}
	
	@When("I go to tripadvisor login page")
	public void pressLogin(){
		tripadvisor.goToLogin();
		Assert.assertTrue("Expected to be at login page", tripadvisor.checkAtLoginPage());
		
	}
	
	@When("I logout tripadvisor")
	public void pressLogout(){
		tripadvisor.logout();
	}
}
