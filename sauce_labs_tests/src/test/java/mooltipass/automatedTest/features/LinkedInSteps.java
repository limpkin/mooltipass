package mooltipass.automatedTest.features;

import cucumber.api.java.en.Then;
import cucumber.api.java.en.When;
import junit.framework.Assert;
import mooltipass.automatedTest.config.WebDriverFactory;
import mooltipass.automatedTest.pageObjects.LinkedIn;

public class LinkedInSteps {
	LinkedIn link= new LinkedIn(WebDriverFactory.get());

	@When("I login LinkedIn with '(.*)'")
	public void login(String username){

		link.enterEmail(username);
		String password =System.getenv().get("LINKEDINPASS");
		link.enterPassword(password);
		link.submit();
		
	}
	@When("I go to LinkedIn login page")
	public void pressLogin(){
		Assert.assertTrue("Expected to be at login page", link.checkAtLoginPage());
		
	}
	@Then("I should be logged in LinkedIn")
	public void checkLogin(){
		Assert.assertTrue("Expected User to be logged in",link.checkLogin());
	}
	
	@When("I logout LinkedIn")
	public void pressLogout(){
		link.goTodDashboard();
		link.logout();
	}
}
